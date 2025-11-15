// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract ImageProof {
    struct ExecutionProof {
        bytes32 preActionHash;
        bytes32 postActionHash;
        uint256 timestamp;
        address device;
        string command;
        bool completed;
    }
    
    mapping(uint256 => ExecutionProof) public proofs;
    uint256 public proofCount;
    
    event CommandExecuted(
        uint256 indexed proofId,
        address indexed device,
        string command,
        bytes32 preHash,
        bytes32 postHash
    );
    
    function storeExecutionProof(
        string memory command,
        bytes32 preHash,
        bytes32 postHash
    ) public {
        proofs[proofCount] = ExecutionProof({
            preActionHash: preHash,
            postActionHash: postHash,
            timestamp: block.timestamp,
            device: msg.sender,
            command: command,
            completed: true
        });
        
        emit CommandExecuted(proofCount, msg.sender, command, preHash, postHash);
        proofCount++;
    }
    
    function getProof(uint256 proofId) public view returns (ExecutionProof memory) {
        return proofs[proofId];
    }
    
    function verifyExecution(uint256 proofId) public view returns (bool) {
        ExecutionProof memory proof = proofs[proofId];
        return proof.completed && 
               proof.preActionHash != proof.postActionHash &&
               proof.timestamp > 0;
    }
}
